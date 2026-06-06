#include "a21100/m21100.h"
QVector<double> m21100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
