#include "m15652/m15652.h"
QVector<double> m15652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
