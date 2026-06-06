#include "c18582/m18582.h"
QVector<double> m18582::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
