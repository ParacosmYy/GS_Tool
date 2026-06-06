#include "f28065/m28065.h"
QVector<double> m28065::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
