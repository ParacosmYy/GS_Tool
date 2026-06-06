#include "p15255/m15255.h"
QVector<double> m15255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
