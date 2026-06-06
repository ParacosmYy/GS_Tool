#include "m34532/m34532.h"
QVector<double> m34532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
