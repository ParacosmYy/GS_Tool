#include "i36708/m36708.h"
QVector<double> m36708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
