#include "m21432/m21432.h"
QVector<double> m21432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
