#include "a19220/m19220.h"
QVector<double> m19220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
