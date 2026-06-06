#include "q8616/m8616.h"
QVector<double> m8616::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
