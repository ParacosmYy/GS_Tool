#include "b25501/m25501.h"
QVector<double> m25501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
