#include "o18114/m18114.h"
QVector<double> m18114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
