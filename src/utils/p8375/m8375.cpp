#include "p8375/m8375.h"
QVector<double> m8375::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
