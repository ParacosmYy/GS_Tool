#include "h8367/m8367.h"
QVector<double> m8367::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
