#include "p8155/m8155.h"
QVector<double> m8155::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
