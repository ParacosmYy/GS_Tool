#include "f9285/m9285.h"
QVector<double> m9285::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
