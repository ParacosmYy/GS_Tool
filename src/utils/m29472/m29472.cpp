#include "m29472/m29472.h"
QVector<double> m29472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
