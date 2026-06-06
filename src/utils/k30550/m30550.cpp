#include "k30550/m30550.h"
QVector<double> m30550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
