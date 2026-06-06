#include "i19588/m19588.h"
QVector<double> m19588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
