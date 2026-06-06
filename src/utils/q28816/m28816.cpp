#include "q28816/m28816.h"
QVector<double> m28816::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
