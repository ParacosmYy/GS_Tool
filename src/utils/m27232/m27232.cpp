#include "m27232/m27232.h"
QVector<double> m27232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
