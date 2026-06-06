#include "p21835/m21835.h"
QVector<double> m21835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
