#include "k22050/m22050.h"
QVector<double> m22050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
