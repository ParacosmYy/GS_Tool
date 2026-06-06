#include "k29410/m29410.h"
QVector<double> m29410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
