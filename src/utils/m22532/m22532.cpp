#include "m22532/m22532.h"
QVector<double> m22532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
