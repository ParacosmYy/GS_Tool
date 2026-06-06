#include "e8864/m8864.h"
QVector<double> m8864::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
