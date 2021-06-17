#ifndef NETIMAGEUTIL_H
#define NETIMAGEUTIL_H

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QNetworkReply>
#include <QPainter>
#include <QFile>
#include <math.h>

class NetImageUtil : QObject
{
public:
    static QPixmap loadNetPixmap(QString url)
    {
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        QByteArray jpegData = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(jpegData);
        return pixmap;
    }

    /// 保存网络文件
    /// 直接存储原数据，可以是图片、视频、文件等等
    /// 有些图片不一定是静态的，所以没法使用 QPixmap
    static void saveNetFile(QString url, QString path)
    {
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        QByteArray data = reply->readAll();
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
    }

    /// 转换为圆角头像
    /// radius: 半径，-1 时为圆形
    static QPixmap toRoundedPixmap(const QPixmap &pixmap, int radius = -1)
    {
        QPixmap dest(pixmap.size());
        dest.fill(Qt::transparent);
        QPainter painter(&dest);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        QRect rect = QRect(0, 0, pixmap.width(), pixmap.height());
        if (radius == -1)
            radius = qMin(rect.width(), rect.height())/2;
        QPainterPath path;
        path.addRoundedRect(rect, radius, radius);
        painter.setClipPath(path);
        painter.drawPixmap(rect, pixmap);
        return dest;
    }

    /// 绕中心点圆形透明渐变
    static QPixmap toCircleTransparentGradient(const QPixmap& pixmap)
    {
        QPixmap rst(pixmap.size());
        rst.fill(Qt::transparent);
        QPainter painter(&rst);

        QImage img = pixmap.toImage();
        QPoint center(pixmap.rect().center());
        int alpha0 = qMin(center.x() * center.x(), center.y()* center.y()); // 这是距离的平方
        int alpha255 = alpha0 / 9; // 确定开始渐变的距离

        for (int x = 0; x < pixmap.width(); x++)
        {
            for (int y = 0; y < pixmap.height(); y++)
            {
                int dis2 = (x - center.x()) * (x - center.x())
                        + (y - center.y()) * (y - center.y()); // 使用sqrt来判断是否需要线性渐变
                int alpha = 0;
                if (dis2 <= alpha255)
                    alpha = 255;
                else if (dis2 >= alpha0)
                    alpha = 0;
                else
                    alpha = 255 - int(255 * double(dis2 - alpha255) / (alpha0 - alpha255));
                QColor c = img.pixelColor(x, y);
                c.setAlpha(alpha);
                painter.setPen(c);
                painter.drawPoint(x, y);
            }
        }
        return rst;
    }

    /// 绕右上角透明渐变
    static QPixmap toCornelTransparentGradient(const QPixmap& pixmap, int radius)
    {
        QPixmap rst(pixmap.size());
        rst.fill(Qt::transparent);
        QPainter painter(&rst);

        // 裁剪右边的圆角
        if (radius)
        {
            QPainterPath path;
            path.addRect(0, 0, radius+1, pixmap.height());
            path.addRoundedRect(0, 0, pixmap.width(), pixmap.height(), radius, radius);
            painter.setClipPath(path);
        }

        // 设置透明度变化
        QImage img = pixmap.toImage();
        int w = pixmap.width(), h = pixmap.height();
        double effc = 2.5;
        int alpha0 = int(qMin(pow(w, effc), pow(h, effc))); // 这是距离的平方
        int alpha255 = 0; // 确定开始渐变的距离
        int maxAlpha = 200;

        auto getAlpha = [=](int dis2) {
            if (dis2 <= alpha255)
                return maxAlpha;
            else if (dis2 >= alpha0)
                return 0;
            else
                return maxAlpha - int(maxAlpha * double(dis2 - alpha255) / (alpha0 - alpha255));
        };

        // 挨个像素修改透明度
        for (int x = 0; x < w; x++)
        {
            for (int y = 0; y < h; y++)
            {
                int dis = int(pow((w-x), effc) + pow(y, effc)); // 使用sqrt来判断是否需要线性渐变

                QColor c = img.pixelColor(x, y);
                c.setAlpha(getAlpha(dis));
                painter.setPen(c);
                painter.drawPoint(x, y);
            }
        }

        return rst;
    }
};

#endif // NETIMAGEUTIL_H
