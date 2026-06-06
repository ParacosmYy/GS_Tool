#include "m21652/m21652.h"
QVector<double> m21652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
