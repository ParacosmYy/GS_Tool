#include "r8077/m8077.h"
QVector<double> m8077::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
