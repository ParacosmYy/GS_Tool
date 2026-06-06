#include "c25622/m25622.h"
QVector<double> m25622::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
