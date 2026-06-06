#include "p8735/m8735.h"
QVector<double> m8735::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
