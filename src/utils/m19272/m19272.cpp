#include "m19272/m19272.h"
QVector<double> m19272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
