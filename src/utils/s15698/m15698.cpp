#include "s15698/m15698.h"
QVector<double> m15698::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
