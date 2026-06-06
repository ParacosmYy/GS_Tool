#include "n25353/m25353.h"
QVector<double> m25353::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
