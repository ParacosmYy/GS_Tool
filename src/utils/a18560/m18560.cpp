#include "a18560/m18560.h"
QVector<double> m18560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
