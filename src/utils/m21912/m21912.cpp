#include "m21912/m21912.h"
QVector<double> m21912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
