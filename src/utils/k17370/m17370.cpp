#include "k17370/m17370.h"
QVector<double> m17370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
