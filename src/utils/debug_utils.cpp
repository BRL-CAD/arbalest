
void print(QObject * w, int depth){
    for(int c=0;c<depth;c++)std::cout<<"   ";
    if(w && w->metaObject())std::cout<<w->metaObject()->className()<<std::endl;
    for(int i=0;i<w->children().length();i++){



        QWidget *y = dynamic_cast<QWidget *>(w->children().at(i));
        if( y&&QString(y->metaObject()->className())== QString("QWidget")) {
            y->setVisible(false);
            //std::cout<<y->layout()->children().length()<<std::endl;
            std::cout << "-";
        }

        print(w->children().at(i),depth+1);

    }
}

void TintImage(QImage& inoutImage, const QColor& tintColor)
{
    if (tintColor == Qt::white)
        return;

    // Convert to 4-channel 32-bit format if needed
    auto format = inoutImage.format();
    if (format != QImage::Format_ARGB32 && format != QImage::Format_ARGB32_Premultiplied)
    {
        format = QImage::Format_ARGB32_Premultiplied;
        inoutImage = inoutImage.convertToFormat(format);
    }

    const bool isPremultiplied = (format == QImage::Format_ARGB32_Premultiplied);
    const auto tint = tintColor.rgba();

    // Convert scanline by scanline (a bit tricker than using setPixelColor, but much more efficient)
    const int sizeX = inoutImage.width();
    const int sizeY = inoutImage.height();
    for (int y = 0; y < sizeY; ++y)
    {
        // Note: Qt documentation explicitly recommends this cast for 32-bit images
        auto* scanline = (QRgb*)inoutImage.scanLine(y);
        for (int x = 0; x < sizeX; ++x)
        {
            auto color = scanline[x];
            if (isPremultiplied)
                color = qUnpremultiply(color);

            color = qRgba(
                (qRed(color) * qRed(tint)) / 255
                , (qGreen(color) * qGreen(tint)) / 255
                , (qBlue(color) * qBlue(tint)) / 255
                , (qAlpha(color) * qAlpha(tint)) / 255
            );

            if (isPremultiplied)
                color = qPremultiply(color);

            scanline[x] = color;
        }
    }
}