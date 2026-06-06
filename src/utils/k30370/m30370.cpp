#include "k30370/m30370.h"
QVector<double> m30370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
