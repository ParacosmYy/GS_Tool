#include "l35511/m35511.h"
QVector<double> m35511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
