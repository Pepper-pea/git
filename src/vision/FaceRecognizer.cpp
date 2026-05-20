#include "vision/FaceRecognizer.h" // 引入人脸识别器声明。
#include "utils/MatImageConverter.h" // 引入 Qt 与 OpenCV 图像转换工具。
#include <QDebug> // 引入调试输出。
#include <QFileInfo> // 引入文件信息。
#include <QtMath> // 引入 Qt 数学函数。
#include <algorithm> // 引入排序算法。
#include <exception> // 引入标准异常类型。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
#include <opencv2/imgproc.hpp> // 引入 OpenCV 图像预处理。
#endif // 结束 OpenCV 判断。
#ifdef SMARTSITE_HAS_DLIB // 判断是否启用 dlib。
#include <dlib/array2d.h> // 引入 dlib 图像容器。
#include <dlib/pixel.h> // 引入 dlib RGB 像素类型。
#endif // 结束 dlib 判断。
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
void FaceRecognizer::setEarThreshold(double threshold) { // 实现 EAR 阈值设置函数。
    liveness_.setEarThreshold(threshold); // 转发给活体检测器。
} // 结束 EAR 阈值设置函数。
void FaceRecognizer::setUseDlib(bool enabled) { // 实现 dlib 开关设置函数。
    useDlib_ = enabled; // 保存 dlib 开关。
} // 结束开关设置函数。
bool FaceRecognizer::livenessAvailable() const { // 实现 dlib 68 点活体可用性判断。
    return useDlib_ && shapePredictorLoaded_; // 仅模型实际加载后才认为活体可用。
} // 结束活体可用性判断。
QString FaceRecognizer::lastError() const { // 实现最近失败原因读取函数。
    return lastError_; // 返回最近一次失败原因。
} // 结束失败原因读取函数。
void FaceRecognizer::setModelPaths(const QString& faceCascadePath, const QString& shapePredictorPath) { // 实现模型路径设置函数。
    faceCascadePath_ = faceCascadePath; // 保存 OpenCV Haar 模型路径。
    shapePredictorPath_ = shapePredictorPath; // 保存 dlib 68 点模型路径。
} // 结束模型路径设置函数。
bool FaceRecognizer::loadModels() { // 实现模型加载函数。
    bool loadedAnyModel = false; // 记录是否至少加载了一个真实模型。
    haarLoaded_ = false; // 重置 Haar 加载状态。
    shapePredictorLoaded_ = false; // 重置 dlib 加载状态。
    lastError_.clear(); // 清空旧错误。
    emit logMessage(QStringLiteral("INFO"), QStringLiteral("模型路径：Haar=%1；dlib=%2").arg(faceCascadePath_, shapePredictorPath_)); // 输出模型路径便于排查。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    if(faceCascadePath_.isEmpty() || !QFileInfo::exists(faceCascadePath_)) { // 判断 Haar 模型文件是否存在。
        lastError_ = QStringLiteral("OpenCV Haar 模型不存在：%1").arg(faceCascadePath_); // 保存错误原因。
        emit logMessage(QStringLiteral("ERROR"), QStringLiteral("OpenCV Haar 模型不存在：%1").arg(faceCascadePath_)); // 发出错误日志。
    } else if(faceCascade_.load(faceCascadePath_.toStdString())) { // 加载 Haar 模型。
        haarLoaded_ = true; // 标记 Haar 模型已加载。
        loadedAnyModel = true; // 标记已加载真实模型。
        emit logMessage(QStringLiteral("INFO"), QStringLiteral("OpenCV Haar 模型已加载：%1").arg(faceCascadePath_)); // 发出成功日志。
    } else { // 处理 Haar 模型加载失败。
        lastError_ = QStringLiteral("OpenCV Haar 模型加载失败：%1").arg(faceCascadePath_); // 保存错误原因。
        emit logMessage(QStringLiteral("ERROR"), QStringLiteral("OpenCV Haar 模型加载失败：%1").arg(faceCascadePath_)); // 发出失败日志。
    } // 结束 Haar 模型加载判断。
#else // 处理未启用 OpenCV 的构建。
    lastError_ = QStringLiteral("当前构建未启用 OpenCV，无法使用 Haar 人脸检测模型"); // 保存错误原因。
    emit logMessage(QStringLiteral("ERROR"), QStringLiteral("当前构建未启用 OpenCV，无法使用 Haar 人脸检测模型")); // 发出错误日志。
#endif // 结束 OpenCV 判断。
#ifdef SMARTSITE_HAS_DLIB // 判断是否启用 dlib。
    if(useDlib_) { // 判断是否要求使用 dlib。
        if(shapePredictorPath_.isEmpty() || !QFileInfo::exists(shapePredictorPath_)) { // 判断 dlib 68 点模型文件是否存在。
            emit logMessage(QStringLiteral("WARN"), QStringLiteral("dlib 68 点模型不存在：%1，普通识别可用，活体动作录入不可用").arg(shapePredictorPath_)); // 发出降级日志。
        } else { // 处理模型文件存在。
            try { // 捕获 dlib 反序列化异常。
                dlib::deserialize(shapePredictorPath_.toStdString()) >> shapePredictor_; // 加载 dlib 68 点模型。
                shapePredictorLoaded_ = true; // 标记 dlib 模型已加载。
                loadedAnyModel = true; // 标记已加载真实模型。
                emit logMessage(QStringLiteral("INFO"), QStringLiteral("dlib 68 点模型已加载：%1").arg(shapePredictorPath_)); // 发出成功日志。
            } catch(const std::exception& error) { // 处理模型加载异常。
                emit logMessage(QStringLiteral("WARN"), QStringLiteral("dlib 68 点模型加载失败：%1，普通识别可用，活体动作录入不可用").arg(QString::fromLocal8Bit(error.what()))); // 发出降级日志。
            } // 结束异常处理。
        } // 结束文件存在判断。
    } // 结束 dlib 启用判断。
#else // 处理未启用 dlib 的构建。
    if(useDlib_) { // 判断配置是否要求 dlib。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("当前构建未启用 dlib，普通识别可用，活体动作录入不可用")); // 发出降级日志。
    } // 结束 dlib 配置判断。
#endif // 结束 dlib 判断。
    return loadedAnyModel; // 返回模型加载结果。
} // 结束模型加载函数。
QVector<FaceDetection> FaceRecognizer::detectFaces(const QImage& image) const { // 实现人脸检测函数。
    QVector<FaceDetection> faces; // 创建检测结果数组。
    if(image.isNull()) { // 判断输入图像是否为空。
        return faces; // 返回空结果。
    } // 结束空图像判断。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
    if(haarLoaded_) { // 判断 Haar 模型是否已加载。
        return detectFacesWithHaar(image); // 使用真实 Haar 模型检测人脸。
    } // 结束 Haar 模型判断。
#endif // 结束 OpenCV 判断。
    return faces; // 未加载真实检测模型时返回空结果。
} // 结束人脸检测函数。
RecognitionResult FaceRecognizer::recognize(const QImage& image) { // 实现识别函数。
    RecognitionResult result; // 创建识别结果对象。
    result.createdAt = QDateTime::currentDateTime(); // 记录当前时间。
    lastError_.clear(); // 清空旧错误。
    if(image.isNull()) { // 判断输入图像是否为空。
        result.status = RecognitionStatus::LowQuality; // 标记图像质量不足。
        result.message = QStringLiteral("输入图像为空"); // 记录提示文本。
        lastError_ = result.message; // 保存失败原因。
        return result; // 返回结果。
    } // 结束图像判断。
    const QVector<FaceDetection> faces = detectFaces(image); // 检测人脸。
    if(faces.isEmpty()) { // 判断是否未检测到人脸。
        result.status = RecognitionStatus::Stranger; // 标记陌生人。
        result.message = haarLoaded_ ? QStringLiteral("未检测到人脸") : QStringLiteral("OpenCV Haar 模型未加载，无法检测人脸"); // 记录提示文本。
        lastError_ = result.message; // 保存失败原因。
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
        lastError_ = result.message; // 保存失败原因。
        return result; // 返回结果。
    } // 结束质量判断。
    if(livenessAvailable() && result.detection.landmarks.size() >= 68) { // 判断是否可以记录眨眼活体状态。
        result.detection.blinkDetected = liveness_.update(result.detection); // 记录眨眼结果，但默认不拦截普通识别。
    } // 结束活体状态记录。
    return matchAgainstDatabase(face, result.detection); // 返回数据库匹配结果。
} // 结束识别函数。
bool FaceRecognizer::enrollPerson(Person person, const QImage& image) { // 实现人员特征录入函数。
    lastError_.clear(); // 清空旧错误。
    if(image.isNull() || person.id.isEmpty()) { // 判断图像和人员编号是否有效。
        lastError_ = QStringLiteral("图像或编号无效"); // 保存失败原因。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：图像或编号无效")); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束输入检查。
    const QVector<FaceDetection> faces = detectFaces(image); // 检测当前图像中的人脸。
    if(faces.isEmpty()) { // 判断是否没有检测到人脸。
        lastError_ = haarLoaded_ ? QStringLiteral("未检测到人脸，请让人脸位于画面中央并保持光线充足") : QStringLiteral("OpenCV Haar 模型未加载，无法检测人脸"); // 保存失败原因。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：%1").arg(lastError_)); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束人脸检查。
    if(livenessAvailable() && faces.first().landmarks.size() < 68) { // 判断是否缺少真实 dlib 68 点关键点。
        lastError_ = QStringLiteral("dlib 68 点关键点未就绪"); // 保存失败原因。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：dlib 68 点关键点未就绪")); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束 dlib 关键点判断。
    const QRect faceRect = faces.first().rect.intersected(image.rect()); // 计算安全的人脸区域。
    person.feature = extractor_.extract(image, faceRect); // 提取并写入人员特征。
    if(person.feature.isEmpty()) { // 判断特征是否提取成功。
        lastError_ = QStringLiteral("特征提取失败"); // 保存失败原因。
        emit logMessage(QStringLiteral("WARN"), QStringLiteral("人员录入失败：特征提取失败")); // 发出失败日志。
        return false; // 返回录入失败。
    } // 结束特征检查。
    const bool saved = DatabaseManager::instance().upsertPerson(person); // 保存人员到数据库。
    if(!saved) { // 判断数据库保存是否失败。
        lastError_ = DatabaseManager::instance().lastError(); // 保存数据库失败原因。
    } // 结束保存失败判断。
    emit logMessage(saved ? QStringLiteral("INFO") : QStringLiteral("ERROR"), saved ? QStringLiteral("人员已录入：%1").arg(person.name) : DatabaseManager::instance().lastError()); // 发出保存结果日志。
    return saved; // 返回保存结果。
} // 结束人员特征录入函数。
QVector<Person> FaceRecognizer::enrolledPersons() const { // 实现已注册人员查询函数。
    return DatabaseManager::instance().loadPersons(); // 直接读取数据库中的人员列表。
} // 结束人员查询函数。
#ifdef SMARTSITE_HAS_OPENCV // 判断是否启用 OpenCV。
QVector<FaceDetection> FaceRecognizer::detectFacesWithHaar(const QImage& image) const { // 实现 OpenCV Haar 人脸检测函数。
    QVector<FaceDetection> detections; // 创建检测结果数组。
    cv::Mat bgr = MatImageConverter::qImageToMat(image); // 将 Qt 图像转换为 OpenCV 图像。
    if(bgr.empty()) { // 判断转换是否成功。
        return detections; // 转换失败时返回空结果。
    } // 结束转换判断。
    cv::Mat gray; // 创建灰度图。
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY); // 转换为灰度。
    cv::equalizeHist(gray, gray); // 均衡化提高检测稳定性。
    std::vector<cv::Rect> faces; // 创建 OpenCV 检测框数组。
    faceCascade_.detectMultiScale(gray, faces, 1.1, 4, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(80, 80)); // 使用 Haar 模型检测人脸。
    for(const cv::Rect& face : faces) { // 遍历检测到的人脸框。
        FaceDetection detection; // 创建检测结果。
        detection.rect = QRect(face.x, face.y, face.width, face.height).intersected(image.rect()); // 保存人脸框。
        detection.quality = evaluateQuality(image.copy(detection.rect)); // 保存图像质量。
#ifdef SMARTSITE_HAS_DLIB // 判断是否启用 dlib。
        if(useDlib_ && shapePredictorLoaded_) { // 判断是否需要补齐 68 点关键点。
            fillDlibLandmarks(image, detection); // 使用 dlib 68 点模型提取关键点。
        } // 结束 dlib 判断。
#endif // 结束 dlib 判断。
        detections.append(detection); // 追加检测结果。
    } // 结束检测框遍历。
    std::sort(detections.begin(), detections.end(), [](const FaceDetection& left, const FaceDetection& right) { return left.rect.width() * left.rect.height() > right.rect.width() * right.rect.height(); }); // 按人脸面积从大到小排序。
    return detections; // 返回检测结果。
} // 结束 OpenCV Haar 人脸检测函数。
#endif // 结束 OpenCV 判断。
#ifdef SMARTSITE_HAS_DLIB // 判断是否启用 dlib。
void FaceRecognizer::fillDlibLandmarks(const QImage& image, FaceDetection& detection) const { // 实现 dlib 68 点关键点提取函数。
    if(image.isNull() || detection.rect.isEmpty()) { // 判断输入是否有效。
        return; // 无效时直接返回。
    } // 结束输入判断。
    const QRect faceRect = detection.rect.intersected(image.rect()); // 约束人脸框到图像范围。
    if(faceRect.isEmpty()) { // 判断人脸框是否有效。
        return; // 无效时直接返回。
    } // 结束人脸框判断。
    const QImage rgbImage = image.convertToFormat(QImage::Format_RGB888); // 转成 RGB888。
    dlib::array2d<dlib::rgb_pixel> dlibImage; // 创建 dlib 图像。
    dlibImage.set_size(rgbImage.height(), rgbImage.width()); // 设置 dlib 图像大小。
    for(int y = 0; y < rgbImage.height(); ++y) { // 遍历图像行。
        const uchar* line = rgbImage.constScanLine(y); // 读取当前行。
        for(int x = 0; x < rgbImage.width(); ++x) { // 遍历图像列。
            const int offset = x * 3; // 计算 RGB 偏移。
            dlibImage[y][x] = dlib::rgb_pixel(line[offset], line[offset + 1], line[offset + 2]); // 写入 dlib 像素。
        } // 结束列遍历。
    } // 结束行遍历。
    const dlib::rectangle rectangle(faceRect.left(), faceRect.top(), faceRect.right(), faceRect.bottom()); // 创建 dlib 人脸矩形。
    const dlib::full_object_detection shape = shapePredictor_(dlibImage, rectangle); // 执行 68 点关键点预测。
    detection.landmarks.clear(); // 清空旧关键点。
    detection.landmarks.reserve(static_cast<int>(shape.num_parts())); // 预留关键点容量。
    for(unsigned long index = 0; index < shape.num_parts(); ++index) { // 遍历关键点。
        detection.landmarks.append(QPointF(shape.part(index).x(), shape.part(index).y())); // 保存关键点坐标。
    } // 结束关键点遍历。
} // 结束 dlib 68 点关键点提取函数。
#endif // 结束 dlib 判断。
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
        lastError_ = best.message; // 保存失败原因。
        return best; // 返回结果。
    } // 结束特征判断。
    double bestCosine = -1.0; // 创建最佳余弦值。
    double bestEuclidean = 999.0; // 创建最佳欧氏距离。
    int comparedCount = 0; // 记录参与匹配的人员模板数量。
    Person bestPerson; // 创建最佳人员对象。
    for(const Person& person : persons) { // 遍历数据库人员。
        if(!person.enabled) { // 判断人员是否启用。
            continue; // 跳过未启用人员。
        } // 结束启用判断。
        if(person.feature.isEmpty()) { // 判断是否有模板特征。
            continue; // 跳过没有模板的人。
        } // 结束模板判断。
        ++comparedCount; // 记录一个可用模板。
        const double cosine = FaceFeatureExtractor::cosineSimilarity(feature, person.feature); // 计算余弦相似度。
        const double euclidean = FaceFeatureExtractor::euclideanDistance(feature, person.feature); // 计算欧氏距离。
        if(cosine > bestCosine) { // 判断是否比当前最佳更优。
            bestCosine = cosine; // 记录更高余弦值。
            bestEuclidean = euclidean; // 记录对应欧氏距离。
            bestPerson = person; // 记录对应人员。
        } // 结束比较判断。
    } // 结束遍历人员。
    if(comparedCount == 0) { // 判断是否没有可用人员模板。
        best.status = RecognitionStatus::Stranger; // 标记陌生人。
        best.message = QStringLiteral("人员库为空或没有可用人脸模板"); // 记录提示文本。
        best.cosine = 0.0; // 保存默认余弦值。
        best.euclidean = 999.0; // 保存默认欧氏距离。
        lastError_ = best.message; // 保存失败原因。
        return best; // 返回结果。
    } // 结束模板数量判断。
    if(bestCosine < cosineThreshold_ || bestEuclidean > euclideanThreshold_) { // 判断是否达到识别阈值。
        best.status = RecognitionStatus::Stranger; // 标记陌生人。
        best.message = QStringLiteral("未达到识别阈值：cosine=%1/%2, euclidean=%3/%4").arg(QString::number(bestCosine, 'f', 3), QString::number(cosineThreshold_, 'f', 3), QString::number(bestEuclidean, 'f', 3), QString::number(euclideanThreshold_, 'f', 3)); // 记录提示文本。
        best.cosine = bestCosine; // 保存最佳余弦值。
        best.euclidean = bestEuclidean; // 保存最佳欧氏距离。
        lastError_ = best.message; // 保存失败原因。
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
