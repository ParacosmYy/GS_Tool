#include "e8804/m8804.h"
QVector<double> m8804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
