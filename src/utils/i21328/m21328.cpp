#include "i21328/m21328.h"
QVector<double> m21328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
