#include "p28035/m28035.h"
QVector<double> m28035::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
