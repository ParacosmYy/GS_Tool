#include "c32522/m32522.h"
QVector<double> m32522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
