#include "i19908/m19908.h"
QVector<double> m19908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
