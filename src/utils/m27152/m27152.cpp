#include "m27152/m27152.h"
QVector<double> m27152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
