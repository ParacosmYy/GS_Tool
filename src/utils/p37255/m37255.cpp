#include "p37255/m37255.h"
QVector<double> m37255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
