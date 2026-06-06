#include "b21601/m21601.h"
QVector<double> m21601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
