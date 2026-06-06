#include "k23110/m23110.h"
QVector<double> m23110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
