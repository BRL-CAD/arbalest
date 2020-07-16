
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
