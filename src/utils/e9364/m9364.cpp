#include "e9364/m9364.h"
QVector<double> m9364::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
