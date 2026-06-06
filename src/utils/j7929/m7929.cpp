#include "j7929/m7929.h"
QVector<double> m7929::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
