#include "b15501/m15501.h"
QVector<double> m15501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
