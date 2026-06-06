#include "f30385/m30385.h"
QVector<double> m30385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
