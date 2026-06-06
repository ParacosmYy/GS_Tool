#include "c8522/m8522.h"
QVector<double> m8522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
