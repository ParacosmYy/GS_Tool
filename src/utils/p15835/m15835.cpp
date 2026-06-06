#include "p15835/m15835.h"
QVector<double> m15835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
