#include "q8756/m8756.h"
QVector<double> m8756::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
