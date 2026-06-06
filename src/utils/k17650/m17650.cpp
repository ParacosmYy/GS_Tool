#include "k17650/m17650.h"
QVector<double> m17650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
