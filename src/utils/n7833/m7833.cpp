#include "n7833/m7833.h"
QVector<double> m7833::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
