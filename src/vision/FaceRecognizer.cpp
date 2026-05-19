#include "vision/FaceRecognizer.h" // 引入人脸识别器声明。
#include <QDebug> // 引入调试输出。
#include <QtMath> // 引入 Qt 数学函数。
FaceRecognizer::FaceRecognizer(QObject* parent) : QObject(parent) { // 实现构造函数。
} // 结束构造函数。
void FaceRecognizer::setCosineThreshold(double threshold) { // 实现余弦阈值设置函数。
    cosineThreshold_ = threshold; // 保存余弦阈值。
} // 结束阈值设置函数。
void FaceRecognizer::setEuclideanThreshold(double threshold) { // 实现欧氏阈值设置函数。
    euclideanThreshold_ = threshold; // 保存欧氏阈值。
} // 结束阈值设置函数。
void FaceRecognizer::setQualityThreshold(double threshold) { // 实现质量阈值设置函数。
    qualityThreshold_ = threshold; // 保存质量阈值。
} // 结束阈值设置函数。
void FaceRecognizer::setUseDlib(bool enabled) { // 实现 dlib 开关设置函数。
    useDlib_ = enabled; // 保存 dlib 开关。
} // 结束开关设置函数。
QVector<FaceDetection> FaceRecognizer::detectFaces(const QImage& image) const { // 实现人脸检测函数。
    QVector<FaceDetection> faces; // 创建检测结果数组。
    if(image.isNull()) { // 判断输入图像是否为空。
        return faces; // 返回空结果。
    } // 结束空图像判断。
    FaceDetection detection = buildFallbackDetection(image); // 构造降级检测结果。
    faces.append(detection); // 追加一个默认检测框。
    return faces; // 返回检测结果。
} // 结束人脸检测函数。
RecognitionResult FaceRecognizer::recognize(const QImage& image) { // 实现识别函数。
    RecognitionResult result; // 创建识别结果对象。
    result.createdAt = QDateTime::currentDateTime(); // 记录当前时间。
    if(image.isNull()) { // 判断输入图像是否为空。
        result.status = RecognitionStatus::LowQuality; // 标记图像质量不足。
        result.message = QStringLiteral("输入图像为空"); // 记录提示文本。
        return result; // 返回结果。
    } // 结束图像判断。
    const QVector<FaceDetection> faces = detectFaces(image); // 检测人脸。
    if(faces.isEmpty()) { // 判断是否未检测到人脸。
        result.status = RecognitionStatus::Stranger; // 标记陌生人。
        result.message = QStringLiteral("未检测到人脸"); // 记录提示文本。
        return result; // 返回结果。
    } // 结束人脸判断。
    const FaceDetection detection = faces.first(); // 取第一张人脸作为目标。
    result.detection = detection; // 保存检测结果。
    const QRect faceRect = detection.rect.isValid() ? detection.rect : image.rect().adjusted(image.width() / 4, image.height() / 4, -image.width() / 4, -image.height() / 4); // 计算安全人脸区域。
    const QImage face = image.copy(faceRect.intersected(image.rect())); // 裁剪目标人脸。
    result.detection.quality = evaluateQuality(face); // 计算图像质量。
    if(result.detection.quality < qualityThreshold_) { // 判断图像质量是否过低。
        result.status = RecognitionStatus::LowQuality; // 标记图像质量不足。
        result.message = QStringLiteral("图像质量不足"); // 记录提示文本。
        return result; // 返回结果。
    } // 结束质量判断。
    result.detection.blinkDetected = liveness_.update(result.detection); // 更新活体检测状态。
    if(!result.detection.blinkDetected) { // 判断活体是否通过。
        result.status = RecognitionStatus::Spoof; // 标记为疑似翻拍。
        result.message = QStringLiteral("活体检测未通过"); // 记录提示文本。
        return result; // 返回结果。
    } // 结束活体判断。
    return matchAgainstDatabase(face, result.detection); // 返回数据库匹配结果。
} // 结束识别函数。
bool FaceRecognizer::enrollPerson(Person person, const QImage& image) { // 实现人员特征录入函数。
    if(image.isNull() || person.id.isEmpty()) { // 判断图像和人员编号是否有效。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：图像或编号无效")); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束输入检查。
    const QVector<FaceDetection> faces = detectFaces(image); // 检测当前图像中的人脸。
    if(faces.isEmpty()) { // 判断是否没有检测到人脸。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：未检测到人脸")); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束人脸检查。
    const QRect faceRect = faces.first().rect.intersected(image.rect()); // 计算安全的人脸区域。
    person.feature = extractor_.extract(image, faceRect); // 提取并写入人员特征。
    if(person.feature.isEmpty()) { // 判断特征是否提取成功。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：特征提取失败")); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束特征检查。
    const bool saved = DatabaseManager::instance().upsertPerson(person); // 保存人员到数据库。
    emit logMessage(saved ? QStringLiteral("INFO") : QStringLiteral("ERROR"), saved ? QStringLiteral("人员已录入：%1").arg(person.name) : DatabaseManager::instance().lastError()); // 发出保存结果日志。
    return saved; // 返回保存结果。
} // 结束人员特征录入函数。
QVector<Person> FaceRecognizer::enrolledPersons() const { // 实现已注册人员查询函数。
    return DatabaseManager::instance().loadPersons(); // 直接读取数据库中的人员列表。
} // 结束人员查询函数。
FaceDetection FaceRecognizer::buildFallbackDetection(const QImage& image) const { // 实现降级检测框构造函数。
    FaceDetection detection; // 创建检测结果。
    const int width = qMax(1, image.width() / 2); // 计算默认框宽度。
    const int height = qMax(1, image.height() / 2); // 计算默认框高度。
    detection.rect = QRect((image.width() - width) / 2, (image.height() - height) / 3, width, height); // 将框放在图像中心区域。
    detection.landmarks.reserve(68); // 预留关键点数量。
    for(int index = 0; index < 68; ++index) { // 生成模拟关键点。
        const double x = detection.rect.left() + (index % 8) * qMax(1, detection.rect.width() / 8); // 计算关键点横坐标。
        const double y = detection.rect.top() + (index / 8) * qMax(1, detection.rect.height() / 8); // 计算关键点纵坐标。
        detection.landmarks.append(QPointF(x, y)); // 追加关键点。
    } // 结束关键点生成。
    detection.eyeAspectRatio = 0.25; // 赋予默认 EAR。
    detection.blinkDetected = true; // 赋予默认眨眼状态。
    detection.quality = evaluateQuality(image.copy(detection.rect.intersected(image.rect()))); // 计算默认质量。
    return detection; // 返回降级检测结果。
} // 结束降级检测函数。
double FaceRecognizer::evaluateQuality(const QImage& face) const { // 实现图像质量评估函数。
    if(face.isNull()) { // 判断图像是否为空。
        return 0.0; // 返回零质量。
    } // 结束空图像判断。
    QImage gray = face.convertToFormat(QImage::Format_Grayscale8); // 转换成灰度图。
    const int sampleWidth = qMax(1, gray.width() / 8); // 计算采样宽度。
    const int sampleHeight = qMax(1, gray.height() / 8); // 计算采样高度。
    double sum = 0.0; // 创建亮度总和。
    double sumSquares = 0.0; // 创建亮度平方和。
    int count = 0; // 创建采样计数。
    for(int y = 0; y < gray.height(); y += sampleHeight) { // 逐行采样。
        const uchar* line = gray.constScanLine(y); // 读取当前行。
        for(int x = 0; x < gray.width(); x += sampleWidth) { // 逐列采样。
            const double value = static_cast<double>(line[x]) / 255.0; // 归一化亮度值。
            sum += value; // 累加亮度。
            sumSquares += value * value; // 累加平方亮度。
            ++count; // 计数加一。
        } // 结束列遍历。
    } // 结束行遍历。
    if(count == 0) { // 判断采样是否失败。
        return 0.0; // 返回零质量。
    } // 结束采样判断。
    const double mean = sum / count; // 计算平均亮度。
    const double variance = (sumSquares / count) - (mean * mean); // 计算亮度方差。
    const double contrast = qBound(0.0, variance * 4.0, 1.0); // 将对比度归一化。
    const double brightness = 1.0 - qAbs(mean - 0.5) * 2.0; // 计算亮度平衡度。
    return qBound(0.0, 0.6 * contrast + 0.4 * brightness, 1.0); // 返回综合质量值。
} // 结束图像质量评估函数。
RecognitionResult FaceRecognizer::matchAgainstDatabase(const QImage& face, const FaceDetection& detection) { // 实现数据库匹配函数。
    RecognitionResult best; // 创建最佳结果对象。
    best.status = RecognitionStatus::Stranger; // 默认标记为陌生人。
    best.detection = detection; // 保存检测信息。
    best.createdAt = QDateTime::currentDateTime(); // 记录时间。
    const QVector<Person> persons = DatabaseManager::instance().loadPersons(); // 读取全部人员。
    QVector<double> feature = extractor_.extract(face, face.rect()); // 提取当前人脸特征。
    if(feature.isEmpty()) { // 判断特征是否提取成功。
        best.status = RecognitionStatus::LowQuality; // 标记为质量不足。
        best.message = QStringLiteral("特征提取失败"); // 记录提示文本。
        return best; // 返回结果。
    } // 结束特征判断。
    double bestCosine = -1.0; // 创建最佳余弦值。
    double bestEuclidean = 999.0; // 创建最佳欧氏距离。
    Person bestPerson; // 创建最佳人员对象。
    for(const Person& person : persons) { // 遍历数据库人员。
        if(!person.enabled) { // 判断人员是否启用。
            continue; // 跳过未启用人员。
        } // 结束启用判断。
        if(person.feature.isEmpty()) { // 判断是否有模板特征。
            continue; // 跳过没有模板的人。
        } // 结束模板判断。
        const double cosine = FaceFeatureExtractor::cosineSimilarity(feature, person.feature); // 计算余弦相似度。
        const double euclidean = FaceFeatureExtractor::euclideanDistance(feature, person.feature); // 计算欧氏距离。
        if(cosine > bestCosine) { // 判断是否比当前最佳更优。
            bestCosine = cosine; // 记录更高余弦值。
            bestEuclidean = euclidean; // 记录对应欧氏距离。
            bestPerson = person; // 记录对应人员。
        } // 结束比较判断。
    } // 结束遍历人员。
    if(bestCosine < cosineThreshold_ || bestEuclidean > euclideanThreshold_) { // 判断是否达到识别阈值。
        best.status = RecognitionStatus::Stranger; // 标记陌生人。
        best.message = QStringLiteral("未达到识别阈值"); // 记录提示文本。
        best.cosine = bestCosine; // 保存最佳余弦值。
        best.euclidean = bestEuclidean; // 保存最佳欧氏距离。
        return best; // 返回结果。
    } // 结束阈值判断。
    best.status = bestPerson.listType == QStringLiteral("black") ? RecognitionStatus::Denied : RecognitionStatus::Accepted; // 根据名单类型决定状态。
    best.person = bestPerson; // 保存匹配人员。
    best.cosine = bestCosine; // 保存最佳余弦值。
    best.euclidean = bestEuclidean; // 保存最佳欧氏距离。
    best.score = qBound(0.0, bestCosine - bestEuclidean * 0.5, 1.0); // 计算综合评分。
    best.message = best.status == RecognitionStatus::Accepted ? QStringLiteral("识别通过") : QStringLiteral("黑名单拒绝"); // 生成结果文本。
    return best; // 返回识别结果。
} // 结束数据库匹配函数。
