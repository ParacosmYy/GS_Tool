#include "a32300/m32300.h"
QVector<double> m32300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
