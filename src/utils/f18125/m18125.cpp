#include "f18125/m18125.h"
QVector<double> m18125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
