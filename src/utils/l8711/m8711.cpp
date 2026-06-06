#include "l8711/m8711.h"
QVector<double> m8711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
